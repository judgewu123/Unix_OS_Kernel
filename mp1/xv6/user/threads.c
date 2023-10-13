#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include <user/user.h>
#define NULL 0

static struct thread* current_thread = NULL;
static struct thread* root_thread = NULL;
static int count = 0;

static int id = 1;
static jmp_buf env_st;
// TODO: necessary declares, if any

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    count++;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    t->left = NULL;
    t->right = NULL;
    t->parent = NULL;
    id++;
    return t;
}
void thread_add_runqueue(struct thread *t){
    if(root_thread == NULL) root_thread = t;
    else{
        // TODO
        if(current_thread->left==NULL){
            t->parent = current_thread;
            current_thread->left = t;
        }else if(current_thread->right == NULL){
            t->parent = current_thread;
            current_thread->right = t;
        }else{
            free(t->stack);
            free(t->stack_p);
            free(t);
        }
    }
}
void thread_yield(void){
    // TODO
    int r = setjmp(current_thread->env);
    if(r == 0){
        schedule();
        dispatch();
    }
}
void dispatch(void){
    // TODO
    if(current_thread->buf_set == 0){
        current_thread->buf_set = 1;
        int r = setjmp(current_thread->env);
        if(r==0){
            current_thread->env->sp = (unsigned long)current_thread->stack_p;
            current_thread->env->ra=(unsigned long)current_thread->fp;
            longjmp(current_thread->env, 2);
        }
    }else{
        longjmp(current_thread->env, 2);
    }

}
void schedule(void){
    // TODO
    if(current_thread == NULL && root_thread != NULL){
        current_thread = root_thread;
    }
    else{
        if(current_thread->left!=NULL){
            current_thread = current_thread->left;
        }else if(current_thread->right!=NULL){
            current_thread = current_thread->right;
        }else{//leaf
            if(current_thread->parent->left==current_thread && current_thread->parent->right!=NULL){
                current_thread = current_thread->parent->right;
            }else{
                struct thread *t_tmp = current_thread;
                while(t_tmp!=root_thread){
                    if(t_tmp->parent->left == t_tmp){
                        if(t_tmp->parent->right != NULL){
                            t_tmp = t_tmp->parent->right;
                            break;
                        }else{
                            t_tmp = t_tmp->parent;
                        }
                    }else if(t_tmp->parent->right == t_tmp){
                        t_tmp = t_tmp->parent;
                    }
                }
                current_thread = t_tmp;
            }
        }   
    }

}
void thread_exit(void){
    if(current_thread == root_thread && current_thread->left == NULL && current_thread->right == NULL){
        // TODO
        // Hint: No more thread to execute
        free(current_thread->stack);
        free(current_thread);
        root_thread = NULL;
        current_thread = NULL;
        longjmp(env_st, 3);
    }
    else{
        // TODO
        count--;
        if(current_thread->left == NULL && current_thread->right == NULL){
            struct thread * tmp = current_thread;
            schedule();
            if(tmp->parent->left==tmp) tmp->parent->left = NULL;
            if(tmp->parent->right == tmp) tmp->parent->right = NULL;
            free(tmp->stack);
            free(tmp);
            dispatch();
        }
        else{
            struct thread *t_tmp = current_thread;
            while(1){
                if(t_tmp->right!=NULL){
                    t_tmp = t_tmp->right;
                }else if(t_tmp->left!=NULL){
                    t_tmp = t_tmp->left;
                }else{
                    //find the last node
                    if(count == 2){
                        //stand for there is only 2 nodes
                        current_thread = t_tmp;

                    }else{
                        int side = 0;
                        if(current_thread->parent->right == current_thread) side = 2;
                        if(current_thread->parent->left == current_thread) side = 1;
                        
                        if(side == 1) current_thread->parent->left = t_tmp;
                        else if(side == 2) current_thread->parent->right = t_tmp;
                        
                        if(t_tmp->parent->left == t_tmp) t_tmp->parent->left = NULL;
                        else if(t_tmp->parent->right == t_tmp) t_tmp->parent->right = NULL;
                        
                        t_tmp->parent = current_thread->parent;
                        if(current_thread->left!=NULL && current_thread->left != t_tmp){
                            current_thread->left->parent = t_tmp;
                            t_tmp->left = current_thread->left;
                        }
                        if(current_thread->right!=NULL && current_thread->right != t_tmp){
                            current_thread->right->parent = t_tmp;
                            t_tmp->right = current_thread->right;
                        }
                    }
                    if(current_thread == root_thread) root_thread = t_tmp;
                    free(current_thread->stack);
                    free(current_thread);
                    current_thread = t_tmp;
                    schedule();
                    dispatch();
                    break;
                }
            }
        }
    }
    
}
void thread_start_threading(void){
    // TODO
    schedule();
    int k = setjmp(env_st);
    if(k == 0) dispatch();
}
